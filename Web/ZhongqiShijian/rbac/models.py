from django.db import models
import hashlib

# Create your models here.
class User(models.Model):
    name = models.CharField(max_length=32)
    pwd = models.CharField(max_length=32)
    roles = models.ManyToManyField(to='Role')

    class Meta:
        verbose_name_plural = 'User'

    def __str__(self):
        return self.name

    def save(self, *args, **kwargs):
        md5 = hashlib.md5()
        md5.update(self.pwd.encode())   # md5加密
        self.pwd = md5.hexdigest()
        super(User, self).save(*args, **kwargs)

class Permission(models.Model):
    title = models.CharField(max_length=32)
    url = models.CharField(max_length=128)
    roles = models.ManyToManyField(to='Role')

    class Meta:
        verbose_name_plural = 'Permission'

    def __str__(self):
        return self.title



class Role(models.Model):
    title = models.CharField(max_length=32)

    class Meta:
        verbose_name_plural = 'Role'

    def __str__(self):
        return self.title


